package visitor;

import ast.*;
import com.google.common.collect.Sets;
import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 10/30/16.
 */
public class IdentifierNormalizerVisitorMk2 {

    @Value
    class IdentifierScoper {
        Map<String, String> identifierMapping;
        IdentifierScoper parent;

        public IdentifierScoper() {
            this(null);
        }

        public IdentifierScoper(IdentifierScoper identifierScoper) {
            this.identifierMapping = new HashMap<>();
            this.parent = identifierScoper;
        }

        public String getMappedIdentifier(String oldName) {
            if (identifierMapping.containsKey(oldName)) {
                return identifierMapping.get(oldName);
            } else if (parent != null) {
                return parent.getMappedIdentifier(oldName);
            } else {
                return null;
            }
        }

        public void put(String oldName, String newName) {
            identifierMapping.put(oldName, newName);
        }
    }

    public void process(VariableDeclaration variableDeclaration, IdentifierScoper identifierScoper) {
        for (VariableDeclaration.DeclaredVariable declaredVariable : variableDeclaration.getDeclaredVariables()) {
            PrimaryExpressionIdentifier primaryExpressionIdentifier = declaredVariable.getIdentifier();
            String identifier = primaryExpressionIdentifier.getIdentifier();
            identifierScoper.put(identifier, getNewIdentifier());
            primaryExpressionIdentifier.setIdentifier(identifierScoper.getMappedIdentifier(identifier));
            if (declaredVariable.getInitializer() != null) {
                Touchy touchy = new Touchy();
                touchy.visit(declaredVariable.getInitializer());
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                    String newName = identifierScoper.getMappedIdentifier(stringCollectionEntry.getKey());
                    for (PrimaryExpressionIdentifier expressionIdentifier : stringCollectionEntry.getValue()) {
                        expressionIdentifier.setIdentifier(newName);
                    }
                }
            }
        }
    }

    public void visit(Program p) {
        IdentifierScoper globalScope = new IdentifierScoper();
        for (Declaration declaration : p.getDeclarations()) {
            if (declaration instanceof VariableDeclaration) {
                process(((VariableDeclaration) declaration), globalScope);
            }
        }
        for (Function function : p.getFunction()) {
            visit(function, new IdentifierScoper(globalScope));
        }
    }

    public void visit(Function f, IdentifierScoper identifierScoper) {
        for (Parameter parameter : f.getParameterList().getParameters()) {
            PrimaryExpressionIdentifier formalParameterName = parameter.getFormalParameterName();
            identifierScoper.put(formalParameterName.getIdentifier(), getNewIdentifier());
            formalParameterName.setIdentifier(identifierScoper.getMappedIdentifier(formalParameterName.getIdentifier()));
        }
        StatementIdentifierVisitor statementIdentifierVisitor = new StatementIdentifierVisitor(identifierScoper);
        f.getCompoundStatement().visitEachStatement(statementIdentifierVisitor);
    }

    static class IdentifierReplacer implements Visitor<Expression> {

        IdentifierScoper identifierScoper;

        IdentifierReplacer(IdentifierScoper identifierScoper) {
            this.identifierScoper = identifierScoper;
        }

        @Override
        public void visit(Expression expression) {
            Touchy touchy = new Touchy();
            touchy.visit(expression);
            for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : touchy.getIdentifiers().entrySet()) {
                String newName = identifierScoper.getMappedIdentifier(stringCollectionEntry.getKey());
                for (PrimaryExpressionIdentifier identifier : stringCollectionEntry.getValue()) {
                    identifier.setIdentifier(newName);
                }
            }
        }
    }

    class StatementIdentifierVisitor implements Visitor<Statement> {

        IdentifierScoper identifierScoper;

        StatementIdentifierVisitor(IdentifierScoper identifierScoper) {
            this.identifierScoper = identifierScoper;
        }

        @Override
        public void visit(Statement statement) {
            visit(statement, identifierScoper);
        }

        public void visit(Statement statement, IdentifierScoper identifierScoper) {
            if (statement instanceof CompoundStatement) {
                visit(((CompoundStatement) statement), new IdentifierScoper(identifierScoper));
            } else if (statement instanceof IterationStatementDeclareFor) {
                visit(((IterationStatementDeclareFor) statement), new IdentifierScoper(identifierScoper));
            } else {
                IdentifierReplacer identifierReplacer = new IdentifierReplacer(identifierScoper);
                statement.visitAllExpressions(identifierReplacer);
                statement.visitEachStatement(this);
            }
        }

        public void visit(CompoundStatement statement, IdentifierScoper identifierScoper) {
            Set<String> declaredVariables = new HashSet<>();
            Map<String, String> variableMapping = new HashMap<>();
            for (BlockItem blockItem : statement.getBlockItems()) {
                if (blockItem instanceof VariableDeclaration) {
                    for (VariableDeclaration.DeclaredVariable declaredVariable : ((VariableDeclaration) blockItem).getDeclaredVariables()) {
                        PrimaryExpressionIdentifier identifier = declaredVariable.getIdentifier();
                        if (!declaredVariables.contains(identifier.getIdentifier())) {
                            variableMapping.put(identifier.getIdentifier(), getNewIdentifier());
                        }
                        declaredVariables.add(identifier.getIdentifier());
                    }
                } else {
                    visit((Statement) blockItem, identifierScoper);
                }
            }
        }

        public void visit(IterationStatementDeclareFor statement, IdentifierScoper identifierScoper) {
            process(statement.getVariableDeclaration(), identifierScoper);
            IdentifierReplacer identifierReplacer = new IdentifierReplacer(identifierScoper);
            statement.visitAllExpressions(identifierReplacer);

            StatementIdentifierVisitor statementIdentifierVisitor = new StatementIdentifierVisitor(identifierScoper);

            statement.visitEachStatement(statementIdentifierVisitor);
        }
    }



    @Value
    private static class Touchy implements Visitor<Expression> {
        private Map<String, Collection<PrimaryExpressionIdentifier>> identifiers;

        Touchy() {
            identifiers = new HashMap<>();
        }

        @Override
        public void visit(Expression expression) {
            if (expression instanceof PrimaryExpressionIdentifier) {
                String identifier = ((PrimaryExpressionIdentifier) expression).getIdentifier();
                if (!identifiers.containsKey(identifier)) {
                    identifiers.put(identifier, Sets.newIdentityHashSet());
                }
                identifiers.get(identifier).add(((PrimaryExpressionIdentifier) expression));
            }
            expression.visitNestedExpressions(this);
        }
    }

    public String getNewIdentifier() {
        return "";
    }
}

package IdentifierNormalizer;

import ast.*;
import ast.declaration.VariableDeclaration;
import ast.expression.Expression;
import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.statement.CanContainStatements;
import ast.statement.Statement;
import ast.statement.impl.CompoundStatement;
import ast.statement.impl.IterationStatementDeclareFor;
import com.google.common.collect.Sets;
import lombok.Data;
import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 10/21/16.
 */
@Data
public class FunctionIdentifierNormalizerVisitor {
    private Function function;
    private int variableCounter;
    private Map<BlockItem, Map<String, Collection<PrimaryExpressionIdentifier>>> identifierScopes;
    private Map<String, Collection<PrimaryExpressionIdentifier>> functionScope;

    public FunctionIdentifierNormalizerVisitor(Function function) {
        variableCounter = 0;
        identifierScopes = new IdentityHashMap<>();
        functionScope = new HashMap<>();
        this.function = function;
    }

    public Map<String, Collection<PrimaryExpressionIdentifier>> getGlobalVariableAccesses() {
        HashMap<String, Collection<PrimaryExpressionIdentifier>> functionScopeCopy = new HashMap<>(functionScope);
        for (Parameter parameter : function.getParameterList().getParameters()) {
            if (functionScopeCopy.containsKey(parameter.getFormalParameterName().getIdentifier())) {
                functionScopeCopy.remove(parameter.getFormalParameterName().getIdentifier());
            }
        }
        return functionScopeCopy;
    }

    public void visit() {
        for (Parameter parameter : function.getParameterList().getParameters()) {
            Set<PrimaryExpressionIdentifier> primaryExpressionIdentifiers = Sets.newIdentityHashSet();
            primaryExpressionIdentifiers.add(parameter.getFormalParameterName());
            functionScope.put(parameter.getFormalParameterName().getIdentifier(), primaryExpressionIdentifiers);
        }
        IdentifierFetcher identifierFetcher = new IdentifierFetcher();
        identifierFetcher.visit(function.getCompoundStatement());
        // For all global variable references
        for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifierFetcher.getUndeclaredVariables().entrySet()) {
            functionScope.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
            functionScope.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
        }
        visitForDeclarationsRenames();
    }

    public void visitForDeclarationsRenames() {
        Map<String, String> identifierRename = new HashMap<>();
        for (Parameter parameter : function.getParameterList().getParameters()) {
            identifierRename.put(parameter.getFormalParameterName().getIdentifier(), getNewVariableName());
        }
        renameVariableInScope(identifierRename, functionScope);
        visitForDeclarationsRenames(function.getCompoundStatement());
    }

    public void visitForDeclarationsRenames(Statement statement) {
        if (statement instanceof CompoundStatement) {
            visitForDeclarationsRenames(((CompoundStatement) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            visitForDeclarationsRenames(((IterationStatementDeclareFor) statement));
        } else {
            if (statement instanceof CanContainStatements) {
                for (Statement statement1 : ((CanContainStatements) statement).getStatementNodes()) {
                    visitForDeclarationsRenames(statement1);
                }
            }
        }
    }

    public void visitForDeclarationsRenames(CompoundStatement statement) {
        Map<String, String> variableRenames = new HashMap<>();
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof VariableDeclaration) {
                for (VariableDeclaration.DeclaredVariable declaredVariable : ((VariableDeclaration) blockItem).getDeclaredVariables()) {
                    String oldIdentifier = declaredVariable.getIdentifier().getIdentifier();
                    String newVariableName = getNewVariableName();
                    variableRenames.put(oldIdentifier, newVariableName);
                }
            }
        }
        if (identifierScopes.get(statement) == null) {
            throw new IllegalArgumentException();
        }
        renameVariableInScope(variableRenames, identifierScopes.get(statement));
        for (BlockItem blockItem : statement.getBlockItems()) {
            if (blockItem instanceof Statement) {
                visitForDeclarationsRenames(((Statement) blockItem));
            }
        }
    }

    public void visitForDeclarationsRenames(IterationStatementDeclareFor statement) {
        Map<String, String> variableRenames = new HashMap<>();
        for (VariableDeclaration.DeclaredVariable declaredVariable : statement.getVariableDeclaration().getDeclaredVariables()) {
            String oldIdentifier = declaredVariable.getIdentifier().getIdentifier();
            String newIdentifier = getNewVariableName();
            variableRenames.put(oldIdentifier, newIdentifier);
        }
        renameVariableInScope(variableRenames, identifierScopes.get(statement));
        visitForDeclarationsRenames(statement.getStatement());
    }

    @Value
    private class Touchy implements Visitor<Expression> {
        private Map<String, Collection<PrimaryExpressionIdentifier>> identifiers;

        Touchy() {
            identifiers = new HashMap<>();
        }

        @Override
        public void visit(Expression expression) {
            if (expression instanceof PrimaryExpressionIdentifier) {
                String identifier = ((PrimaryExpressionIdentifier) expression).getIdentifier();
                identifiers.putIfAbsent(identifier, Sets.newIdentityHashSet());
                identifiers.get(identifier).add(((PrimaryExpressionIdentifier) expression));
            }
            expression.visitNestedExpressions(this);
        }
    }

    @Value
    private class IdentifierFetcher implements Visitor<Statement> {

        private Map<String, Collection<PrimaryExpressionIdentifier>> identifiers;
        private Set<String> declaredVariables;

        IdentifierFetcher() {
            identifiers = new HashMap<>();
            declaredVariables = new HashSet<>();
        }

        public void process(List<VariableDeclaration.DeclaredVariable> declaredVariables) {
            for (VariableDeclaration.DeclaredVariable declaredVariable : declaredVariables) {
                PrimaryExpressionIdentifier primaryExpressionIdentifier = declaredVariable.getIdentifier();
                String identifier = primaryExpressionIdentifier.getIdentifier();
                this.identifiers.putIfAbsent(identifier, Sets.newIdentityHashSet());
                this.identifiers.get(identifier).add(primaryExpressionIdentifier);
                this.declaredVariables.add(identifier);
                if (declaredVariable.getInitializer() != null) {
                    Touchy touchy = new Touchy();
                    //declaredVariable.getInitializer().visitNestedExpressions(touchy);
                    touchy.visit(declaredVariable.getInitializer());
                    Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                    for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                        this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                        this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                    }
                }
            }
        }

        public Map<String, Collection<PrimaryExpressionIdentifier>> getUndeclaredVariables() {
            Map<String, Collection<PrimaryExpressionIdentifier>> allIdentifiers = new HashMap<>(identifiers);
            for (String declaredVariable : declaredVariables) {
                allIdentifiers.remove(declaredVariable);
            }
            return allIdentifiers;
        }

        public void visit(IterationStatementDeclareFor statement) {
            process(statement.getVariableDeclaration().getDeclaredVariables());

            if (statement.getCondition() != null) {
                Touchy touchy = new Touchy();
                touchy.visit(statement.getCondition());
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers2 = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers2.entrySet()) {
                    this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
            }

            if (statement.getIteration() != null) {
                Touchy touchy = new Touchy();
                touchy.visit(statement.getIteration());
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers2 = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers2.entrySet()) {
                    this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
            }

            IdentifierFetcher identifierFetcher = new IdentifierFetcher();
            identifierFetcher.visit(statement.getStatement());
            Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = identifierFetcher.getIdentifiers();

            for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifierFetcher.getUndeclaredVariables().entrySet()) {
                this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
            }
            Set<String> removedVariables = new HashSet<>(identifierFetcher.getUndeclaredVariables().keySet());
            for (String removedVariable : removedVariables) {
                identifiers.remove(removedVariable);
            }
            identifierScopes.put(statement, this.identifiers);
        }

        public void visit(CompoundStatement statement) {
            for (BlockItem blockItem : statement.getBlockItems()) {
                if (blockItem instanceof VariableDeclaration) {
                    process(((VariableDeclaration) blockItem).getDeclaredVariables());
                } else if (blockItem instanceof Statement) {
                    IdentifierFetcher identifierFetcher = new IdentifierFetcher();
                    identifierFetcher.visit(((Statement) blockItem));
                    Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = identifierFetcher.getIdentifiers();
                    for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifierFetcher.getUndeclaredVariables().entrySet()) {
                        this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                        this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                    }
                    Set<String> removedKeys = new HashSet<>(identifierFetcher.getUndeclaredVariables().keySet());
                    for (String removedKey : removedKeys) {
                        identifiers.remove(removedKey);
                    }
                }
            }
            identifierScopes.put(statement, this.identifiers);
        }

        @Override
        public void visit(Statement statement) {
            if (statement instanceof IterationStatementDeclareFor) {
                IdentifierFetcher identifierFetcher = new IdentifierFetcher();
                identifierFetcher.visit(((IterationStatementDeclareFor) statement));
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifierFetcher.getUndeclaredVariables().entrySet()) {
                    this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
                //this.identifiers.putAll(identifierFetcher.getUndeclaredVariables());
            } else if (statement instanceof CompoundStatement) {
                IdentifierFetcher identifierFetcher = new IdentifierFetcher();
                identifierFetcher.visit((CompoundStatement) statement);
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifierFetcher.getUndeclaredVariables().entrySet()) {
                    this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
                //this.identifiers.putAll(identifierFetcher.getUndeclaredVariables());
            } else {
                Touchy touchy = new Touchy();
                statement.visitOwnedExpressions(touchy);
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                    this.identifiers.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
                statement.visitEachStatement(this);
            }
        }

    }

    public void visit(CompoundStatement statement) {
        statement.getDeclaredVariables();
    }

    public String getNewVariableName() {
        int size = 1;
        int variableCounterCopy = variableCounter;
        while ((variableCounterCopy / 26) > 1) {
            size += 1;
            variableCounterCopy /= 26;
        }
        char[] charArray = new char[size];
        variableCounterCopy = variableCounter;
        for (int i = size - 1; i >= 0; i--) {
            charArray[i] = (char)('a' + variableCounterCopy % 26);
            variableCounterCopy /= 26;
        }
        variableCounter++;
        return new String(charArray);
    }

    public static void renameVariableInScope(Map<String, String> variableRenames, Map<String, Collection<PrimaryExpressionIdentifier>> identifiers) {
        Map<String, Collection<PrimaryExpressionIdentifier>> newIdentifiers = new HashMap<>();
        for (Map.Entry<String, String> stringStringEntry : variableRenames.entrySet()) {
            Collection<PrimaryExpressionIdentifier> changedVariables = renameVariable(stringStringEntry.getKey(), stringStringEntry.getValue(), identifiers);
            newIdentifiers.put(stringStringEntry.getValue(), changedVariables);
        }
        for (String s : variableRenames.keySet()) {
            identifiers.remove(s);
        }
        identifiers.putAll(newIdentifiers);
    }

    private static Collection<PrimaryExpressionIdentifier> renameVariable(String currentName, String newName, Map<String, Collection<PrimaryExpressionIdentifier>> identifiers) {
        Collection<PrimaryExpressionIdentifier> changedVariableNames = identifiers.get(currentName);
        for (PrimaryExpressionIdentifier primaryExpressionIdentifier : changedVariableNames) {
            primaryExpressionIdentifier.setIdentifier(newName);
        }
        return changedVariableNames;
    }

}

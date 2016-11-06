package visitor;

import ast.*;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import lombok.Value;

import java.util.*;

/**
 * Created by ooee on 10/21/16.
 */
public class IdentifierNormalizerVisitor {
    private int variableCounter;
    private Map<BlockItem, Map<String, Collection<PrimaryExpressionIdentifier>>> identifierScopes;

    public IdentifierNormalizerVisitor() {
        variableCounter = 0;
        identifierScopes = new HashMap<>();
    }

    public void visit(Program p) {
        for (Declaration declaration : p.getDeclarations()) {
            if (declaration instanceof VariableDeclaration) {
                List<VariableDeclaration.DeclaredVariable> declaredVariables = ((VariableDeclaration) declaration).getDeclaredVariables();
            } else {

            }
        }
        for (Function function : p.getFunction()) {
            IdentifierFetcher identifierFetcher = new IdentifierFetcher();
            identifierFetcher.visit(function.getCompoundStatement());
            Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = identifierFetcher.getIdentifiers();

        }
    }

    public void visit(Function f) {
        Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = new HashMap<>();
        for (Parameter parameter : f.getParameterList().getParameters()) {
            Set<PrimaryExpressionIdentifier> primaryExpressionIdentifiers = Sets.newIdentityHashSet();
            primaryExpressionIdentifiers.add(parameter.getFormalParameterName());
            identifiers.put(parameter.getFormalParameterName().getIdentifier(), primaryExpressionIdentifiers);
        }
        IdentifierFetcher identifierFetcher = new IdentifierFetcher();
        identifierFetcher.visit(f.getCompoundStatement());
        System.out.println("Identifiers");
        for (Map.Entry<BlockItem, Map<String, Collection<PrimaryExpressionIdentifier>>> compoundStatementMapEntry : identifierScopes.entrySet()) {
            System.out.println("Compound Statement...");
            for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : compoundStatementMapEntry.getValue().entrySet()) {
                System.out.println(stringCollectionEntry);
            }
        }
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
                if (!identifiers.containsKey(identifier)) {
                    identifiers.put(identifier, Sets.newIdentityHashSet());
                }
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
                if (!this.identifiers.containsKey(identifier)) {
                    this.identifiers.put(identifier, Sets.newIdentityHashSet());
                }
                this.identifiers.get(identifier).add(primaryExpressionIdentifier);
                this.declaredVariables.add(identifier);
                if (declaredVariable.getInitializer() != null) {
                    Touchy touchy = new Touchy();
                    declaredVariable.getInitializer().visitNestedExpressions(touchy);
                    Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                    for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                        if (!this.identifiers.containsKey(stringCollectionEntry.getKey())) {
                            this.identifiers.put(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                        }
                        this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                    }
                }
            }
        }

        @Override
        public void visit(Statement statement) {
            if (statement instanceof IterationStatementDeclareFor) {
                process(((IterationStatementDeclareFor) statement).getVariableDeclaration().getDeclaredVariables());
                Touchy touchy = new Touchy();
                statement.visitAllExpressions(touchy);
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                    if (!this.identifiers.containsKey(stringCollectionEntry.getKey())) {
                        this.identifiers.put(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    }
                    this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                }
                //IdentifierFetcher identifierFetcher = new IdentifierFetcher();
                //statement.visitEachStatement(identifierFetcher);
                statement.visitEachStatement(this);
            } else if (statement instanceof CompoundStatement) {
                CompoundStatement compoundStatement = (CompoundStatement) statement;
                for (BlockItem blockItem : compoundStatement.getBlockItems()) {
                    if (blockItem instanceof Statement) {
                        IdentifierFetcher identifierFetcher = new IdentifierFetcher();
                        //((Statement) blockItem).visitEachStatement(identifierFetcher);
                        identifierFetcher.visit(((Statement) blockItem));
                        Set<String> declaredVariables = identifierFetcher.getDeclaredVariables();
                        Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = identifierFetcher.getIdentifiers();
                        for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                            if (!declaredVariables.contains(stringCollectionEntry.getKey())) {
                                if (!this.identifiers.containsKey(stringCollectionEntry.getKey())) {
                                    this.identifiers.put(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                                }
                                this.identifiers.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
                            }
                        }
                        Set<String> strings = new HashSet<>(identifiers.keySet());
                        for (String string : strings) {
                            if (!declaredVariables.contains(string)) {
                                identifiers.remove(string);
                            }
                        }
                    } else if (blockItem instanceof VariableDeclaration) {
                        process(((VariableDeclaration) blockItem).getDeclaredVariables());
                    }
                }
                identifierScopes.put(((CompoundStatement) statement), identifiers);
            } else {
                Touchy touchy = new Touchy();
                statement.visitAllExpressions(touchy);
                Map<String, Collection<PrimaryExpressionIdentifier>> identifiers = touchy.getIdentifiers();
                for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
                    if (!this.identifiers.containsKey(stringCollectionEntry.getKey())) {
                        this.identifiers.put(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                    }
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
            charArray[i] = (char)('A' + variableCounterCopy % 26);
            variableCounterCopy /= 26;
        }
        variableCounter++;
        return new String(charArray);
    }

    private void renameVariableInScope(Map<String, String> variableRenames, Map<String, Collection<PrimaryExpressionIdentifier>> identifiers) {
        Map<String, Collection<PrimaryExpressionIdentifier>> newIdentifiers = new HashMap<>();
        for (Map.Entry<String, String> stringStringEntry : variableRenames.entrySet()) {
            Collection<PrimaryExpressionIdentifier> changedVariables = renameVariable(stringStringEntry.getKey(), stringStringEntry.getValue(), identifiers);
            newIdentifiers.put(stringStringEntry.getValue(), changedVariables);
        }
        identifiers = newIdentifiers;
    }

    private Collection<PrimaryExpressionIdentifier> renameVariable(String currentName, String newName, Map<String, Collection<PrimaryExpressionIdentifier>> identifiers) {
        Collection<PrimaryExpressionIdentifier> changedVariableNames = identifiers.get(currentName);
        for (PrimaryExpressionIdentifier primaryExpressionIdentifier : changedVariableNames) {
            primaryExpressionIdentifier.setIdentifier(newName);
        }
        return changedVariableNames;
    }

}

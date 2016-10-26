package visitor;

import ast.*;
import com.google.common.collect.Lists;

import java.util.*;

/**
 * Created by ooee on 10/21/16.
 */
public class IdentifierNormalizerVisitor {
    private Map<String, Collection<PrimaryExpressionIdentifier>> identifiers;
    private int variableCounter;

    public IdentifierNormalizerVisitor() {
        identifiers = new HashMap<>();
        variableCounter = 0;
    }

    private void checkExpression(Expression expression) {
        if (expression instanceof PrimaryExpressionIdentifier) {
            String identifier = ((PrimaryExpressionIdentifier) expression).getIdentifier();
            if (!identifiers.containsKey(identifier)) {
                identifiers.put(identifier, new ArrayList<>());
            }
            identifiers.get(identifier).add(((PrimaryExpressionIdentifier) expression));
        }
    }

    public void visit(Function f) {
        identifiers = new HashMap<>();
        Visitor<Void, Expression> digger = new Visitor<Void, Expression>() {
            @Override
            public Collection<Void> visit(Expression expression) {
                checkExpression(expression);
                expression.visitNestedExpressions(this);
                return Lists.newArrayList();
            }
        };

        Visitor<Void, Expression> touchy = new Visitor<Void, Expression>() {
            @Override
            public Collection<Void> visit(Expression expression) {
                checkExpression(expression);
                expression.visitNestedExpressions(digger);
                return Lists.newArrayList();
            }
        };

        Visitor<Void, Statement> shadowingVariables = new Visitor<Void, Statement>() {
            @Override
            public Collection<Void> visit(Statement statement) {
               if (statement instanceof CompoundStatement) {
                  Set<String> declaredVariables = ((CompoundStatement)statement).getDeclaredVariables();
               }
               return null;
            }
        };
        f.getCompoundStatement().visitAllExpressions(touchy);
        System.out.println("Identifiers");
        for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : identifiers.entrySet()) {
            System.out.println(stringCollectionEntry);
        }
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

    private void renameVariableInScope(Map<String, String> variableRenames) {
        Map<String, Collection<PrimaryExpressionIdentifier>> newIdentifiers = new HashMap<>();
        for (Map.Entry<String, String> stringStringEntry : variableRenames.entrySet()) {
            Collection<PrimaryExpressionIdentifier> changedVariables = renameVariable(stringStringEntry.getKey(), stringStringEntry.getValue());
            newIdentifiers.put(stringStringEntry.getValue(), changedVariables);
        }
        this.identifiers = newIdentifiers;
    }

    private Collection<PrimaryExpressionIdentifier> renameVariable(String currentName, String newName) {
        Collection<PrimaryExpressionIdentifier> changedVariableNames = identifiers.get(currentName);
        for (PrimaryExpressionIdentifier primaryExpressionIdentifier : changedVariableNames) {
            primaryExpressionIdentifier.setIdentifier(newName);
        }
        return changedVariableNames;
    }
}

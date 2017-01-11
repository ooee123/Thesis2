package IdentifierNormalizer;

import ast.*;
import ast.expression.impl.PrimaryExpressionIdentifier;
import com.google.common.collect.Sets;

import java.util.*;

/**
 * Created by ooee on 11/13/16.
 */
public class ProgramIdentifierNormalizerVisitor {

    private int variableCounter;
    private Map<String, Collection<PrimaryExpressionIdentifier>> globalScope;
    private Set<String> declaredVariables;

    public ProgramIdentifierNormalizerVisitor() {
        variableCounter = 0;
        globalScope = new HashMap<>();
        declaredVariables = new HashSet<>();
    }

    public void visit(Program p) {
        for (Declaration declaration : p.getDeclarations()) {
            if (declaration instanceof VariableDeclaration) {
                List<VariableDeclaration.DeclaredVariable> declaredVariables = ((VariableDeclaration) declaration).getDeclaredVariables();
                for (VariableDeclaration.DeclaredVariable declaredVariable : declaredVariables) {
                    PrimaryExpressionIdentifier primaryExpressionIdentifier = declaredVariable.getIdentifier();
                    String identifier = primaryExpressionIdentifier.getIdentifier();
                    Set<PrimaryExpressionIdentifier> primaryExpressionIdentifiers = Sets.newIdentityHashSet();
                    primaryExpressionIdentifiers.add(primaryExpressionIdentifier);
                    globalScope.put(identifier, primaryExpressionIdentifiers);
                    this.declaredVariables.add(identifier);
                }
            }
        }
        for (Function function : p.getFunction()) {
            FunctionIdentifierNormalizerVisitor functionIdentifierNormalizerVisitor = new FunctionIdentifierNormalizerVisitor(function);
            functionIdentifierNormalizerVisitor.visit();
            Map<String, Collection<PrimaryExpressionIdentifier>> globalVariableAccesses = functionIdentifierNormalizerVisitor.getGlobalVariableAccesses();
            for (Map.Entry<String, Collection<PrimaryExpressionIdentifier>> stringCollectionEntry : globalVariableAccesses.entrySet()) {
                globalScope.putIfAbsent(stringCollectionEntry.getKey(), Sets.newIdentityHashSet());
                globalScope.get(stringCollectionEntry.getKey()).addAll(stringCollectionEntry.getValue());
            }
        }
        Map<String, String> identifierRename = new HashMap<>();
        for (String s : declaredVariables) {
            identifierRename.putIfAbsent(s, getNewVariableName());
        }
        FunctionIdentifierNormalizerVisitor.renameVariableInScope(identifierRename, globalScope);
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
        return "global_" + new String(charArray);
    }
}

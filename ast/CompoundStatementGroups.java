package ast;

import com.google.common.collect.Lists;
import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class CompoundStatementGroups implements Statement {

    private List<Declaration> declarations;
    private List<Statement> statements;

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        int number = 1;
        for (Declaration declaration : declarations) {
            builder.append("\n");
            builder.append(number);
            builder.append(": ");
            builder.append(declaration.toString());
            number++;
        }
        for (Statement statement : statements) {
            builder.append("\n");
            builder.append(number);
            builder.append(": ");
            builder.append(statement.toString());
            number++;
        }
        builder.append("\n");
        return builder.toString();
    }

    @Override
    public String toCode() {
        List<String> declarationCodes = collectionToCode(declarations);
        List<String> statementCodes = collectionToCode(statements);
        return "{\n" + String.join("\n", declarationCodes) + "\n" + String.join("\n", statementCodes) + "\n}";
    }
/*
    public static CompoundStatementGroups addToEnd(BlockItem toBeAdded, Statement body) {
        if (body instanceof CompoundStatementGroups) {
            CompoundStatementGroups compoundStatement = (CompoundStatementGroups) body;
            compoundStatement.getBlockItems().add(toBeAdded);
            return compoundStatement;
        } else {
            List<BlockItem> compoundStatement = new ArrayList<>();
            compoundStatement.add(body);
            compoundStatement.add(toBeAdded);
            return new CompoundStatementGroups(compoundStatement);
        }
    }
*/
    @Override
    public Set<String> getDependantVariables() {
        Set<String> assignedValues = new HashSet<>();
        Set<String> incomingValues = new HashSet<>();
        for (Statement statement : statements) {
            Set<String> dependantVariables = statement.getDependantVariables();
            dependantVariables.removeAll(assignedValues);
            incomingValues.addAll(dependantVariables);
            assignedValues.addAll(statement.getChangedVariables());
        }
        return incomingValues;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> declaredVariables = getDeclaredVariables();
        Set<String> changedVariables = new HashSet<>();
        for (Statement statement : statements) {
            changedVariables.addAll(statement.getGuaranteedChangedVariables());
        }
        changedVariables.removeAll(declaredVariables);
        return changedVariables;
    }

    public Set<String> getDeclaredVariables() {
        Set<String> declaredVariables = new HashSet<>();
        for (Declaration declaration : declarations) {
            List<Declaration.DeclaredVariable> declaredVariablesList = declaration.getDeclaredVariables();
            for (Declaration.DeclaredVariable declaredVariable : declaredVariablesList) {
                declaredVariables.add(declaredVariable.getIdentifier());
            }
        }
        return declaredVariables;
    }

    @Override
    public boolean isCritical() {
        return statements.stream().anyMatch(stm -> stm.isCritical());
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        for (Statement statement : statements) {
            potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
            potentiallyChangedVariables.removeAll(statement.getGuaranteedChangedVariables());
        }
        potentiallyChangedVariables.removeAll(getDeclaredVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        return;
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        for (Declaration declaration : declarations) {
            declaration.visitAllExpressions(visitor);
        }
        for (Statement statement : statements) {
            statement.visitAllExpressions(visitor);
        }
    }
}
package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class SelectionStatementSwitch implements SelectionStatement, CanContainStatements {
    @NonNull private Expression expression;
    @NonNull private Statement statement;

    @Override
    public String toCode() {
        return String.format("switch (%s) %s", expression.toCode(), statement.toCode());
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(expression.getVariables());
        dependentVariables.addAll(statement.getDependantVariables());
        return dependentVariables;
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(expression.getGuaranteedChangedVariables());
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }
*/
    @Override
    public Set<String> getGuaranteedChangedVariables() {
        throw new UnsupportedOperationException();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        throw new UnsupportedOperationException();
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}
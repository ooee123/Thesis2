package ast;

import lombok.AllArgsConstructor;
import lombok.Data;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementDoWhile implements IterationStatement {
    private Statement statement;
    private Expression condition;

    @Override
    public String toCode() {
        return String.format("do %s while (%s)", statement.toCode(), condition.toCode());
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(condition.getVariables());
        dependentVariables.addAll(statement.getDependantVariables());
        return dependentVariables;
    }

    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(condition.getChangedVariables());
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}

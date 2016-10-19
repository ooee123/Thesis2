package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import pdg.PDGNode;
import pdg.PDGNodeWhile;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementWhile implements IterationStatement {
    @NonNull private Expression condition;
    private Statement statement;

    @Override
    public String toCode() {
        return toCode(condition.toCode(), statement.toCode());
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
        changedVariables.addAll(condition.getGuaranteedChangedVariables());
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return condition.getGuaranteedChangedVariables();
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    public static String toCode(String conditionCode, String statementCode) {
        return String.format("while (%s) %s", conditionCode, statementCode);
    }
}

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
public class IterationStatementFor implements IterationStatement, CanContainStatements {
    private Expression initial;
    private Expression condition;
    private Expression iteration;
    @NonNull private Statement statement;

    public static String toCode(String initialCode, String conditionCode, String iterationCode, String statementCode) {
        return String.format("for (%s; %s; %s) %s", initialCode, conditionCode, iterationCode, statementCode);
    }

    @Override
    public String toCode() {
        String initialString = "", conditionString = "", iterationString = "";
        if (initial != null) {
            initialString = initial.toCode();
        }
        if (condition != null) {
            conditionString = condition.toCode();
        }
        if (iteration != null) {
            iterationString = iteration.toCode();
        }
        return toCode(initialString, conditionString, iterationString, statement.toCode());
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependantVariables = new HashSet<>();
        if (initial != null) {
            dependantVariables.addAll(initial.getVariables());
        }
        if (condition != null) {
            dependantVariables.addAll(condition.getVariables());
        }
        if (iteration != null) {
            dependantVariables.addAll(iteration.getVariables());
        }
        dependantVariables.addAll(statement.getDependantVariables());
        return dependantVariables;
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        if (iteration != null) {
            changedVariables.addAll(iteration.getGuaranteedChangedVariables());
        }
        if (condition != null) {
            changedVariables.addAll(condition.getGuaranteedChangedVariables());
        }
        if (iteration != null) {
            changedVariables.addAll(iteration.getGuaranteedChangedVariables());
        }
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }
*/
    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        guaranteedChangedVariables.addAll(initial.getGuaranteedChangedVariables());
        guaranteedChangedVariables.addAll(condition.getGuaranteedChangedVariables());
        return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(initial.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(condition.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(iteration.getPotentiallyChangedVariables());
        return potentiallyChangedVariables;
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}

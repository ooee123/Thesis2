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
public class IterationStatementFor implements IterationStatement {
    private Expression initial;
    private Expression condition;
    private Expression iteration;
    @NonNull private Statement statement;

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
        return String.format("for (%s; %s; %s) %s", initialString, conditionString, iterationString, statement.toCode());
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

    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        if (iteration != null) {
            changedVariables.addAll(iteration.getChangedVariables());
        }
        if (condition != null) {
            changedVariables.addAll(condition.getChangedVariables());
        }
        if (iteration != null) {
            changedVariables.addAll(iteration.getChangedVariables());
        }
        changedVariables.addAll(statement.getChangedVariables());
        return changedVariables;
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}

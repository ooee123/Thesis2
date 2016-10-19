package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/29/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementDeclareFor implements IterationStatement {
    @NonNull private Declaration declaration;
    private Expression condition;
    private Expression iteration;
    @NonNull private Statement statement;

    public String toCode() {
        String conditionString = "", iterationString = "";
        if (condition != null) {
            conditionString = condition.toCode();
        }
        if (iteration != null) {
            iterationString = iteration.toCode();
        }
        return String.format("for (%s %s; %s) %s", declaration.toCode(), conditionString, iterationString, statement.toCode());
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependantVariables = new HashSet<>();
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                dependantVariables.addAll(declaredVariable.getInitializer().getVariables());
            }
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
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                changedVariables.addAll(declaredVariable.getInitializer().getGuaranteedChangedVariables());
            }
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

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> guaranteedChangedVariables = new HashSet<>();
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                guaranteedChangedVariables.addAll(declaredVariable.getInitializer().getGuaranteedChangedVariables());
            }
        }
        if (condition != null) {
            guaranteedChangedVariables.addAll(condition.getGuaranteedChangedVariables());
        }
        return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                potentiallyChangedVariables.addAll(declaredVariable.getInitializer().getPotentiallyChangedVariables());
            }
        }
        if (condition != null) {
            potentiallyChangedVariables.addAll(condition.getPotentiallyChangedVariables());
        }
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        if (iteration != null) {
            potentiallyChangedVariables.addAll(iteration.getPotentiallyChangedVariables());
        }
        return potentiallyChangedVariables;
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}

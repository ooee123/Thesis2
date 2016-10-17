package ast;

import lombok.Data;
import lombok.NonNull;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
public class SelectionStatementIf implements SelectionStatement {
    @NonNull private Expression condition;
    @NonNull private Statement thenStatement;
    private Statement elseStatement;

    public SelectionStatementIf(Expression condition, Statement thenStatement, Statement elseStatement) {
        this.condition = condition;
        this.thenStatement = thenStatement;
        this.elseStatement = elseStatement;
    }

    public SelectionStatementIf(Expression condition, Statement thenStatement) {
        this(condition, thenStatement, null);
    }

    @Override
    public String toCode() {
        String base = String.format("if (%s) %s", condition.toCode(), thenStatement.toCode());
        if (elseStatement != null) {
            return String.format("%s else %s", base, elseStatement.toCode());
        } else {
            return base;
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(condition.getVariables());
        dependentVariables.addAll(thenStatement.getDependantVariables());
        if (elseStatement != null) {
            dependentVariables.addAll(elseStatement.getDependantVariables());
        }
        return dependentVariables;
    }

    @Override
    public Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(condition.getChangedVariables());
        changedVariables.addAll(thenStatement.getChangedVariables());
        if (elseStatement != null) {
            changedVariables.addAll(elseStatement.getChangedVariables());
        }
        return changedVariables;
    }

    @Override
    public boolean isCritical() {
        //TODO
        return true;
    }
}

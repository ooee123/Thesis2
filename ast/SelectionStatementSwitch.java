package ast;

import com.google.common.collect.Lists;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import visitor.Visitor;

import java.util.ArrayList;
import java.util.Collection;
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

    @Override
    public Collection<Statement> getStatementNodes() {
        return Lists.newArrayList(statement);
    }

    @Override
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return visitor.visit(statement);
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        Collection<T> collection = new ArrayList<>();
        collection.addAll(visitor.visit(expression));
        collection.addAll(statement.visitAllExpressions(visitor));
        return collection;
    }

    @Override
    public int pointValue() {
        return expression.pointValue() + statement.pointValue();
    }
}
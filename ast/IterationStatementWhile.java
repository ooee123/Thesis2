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
public class IterationStatementWhile implements IterationStatement, CanContainStatements {
    @NonNull private Expression condition;
    @NonNull private Statement statement;

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
    public Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>();
        potentiallyChangedVariables.addAll(condition.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getPotentiallyChangedVariables());
        potentiallyChangedVariables.addAll(statement.getGuaranteedChangedVariables());
        return potentiallyChangedVariables;
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
        collection.addAll(visitor.visit(condition));
        collection.addAll(statement.visitAllExpressions(visitor));
        return collection;
    }

    @Override
    public int pointValue() {
        int points = 0;
        points += condition.pointValue();
        points += statement.pointValue();
        return points;
    }
}

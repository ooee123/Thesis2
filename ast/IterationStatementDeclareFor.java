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
 * Created by ooee on 9/29/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementDeclareFor implements IterationStatement, CanContainStatements {
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
    public Collection<Statement> getStatementNodes() {
        return Lists.newArrayList(statement);
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

    @Override
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return visitor.visit(statement);
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        Collection<T> collection = new ArrayList<>();
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                collection.addAll(visitor.visit(declaredVariable.getInitializer()));
            }
        }
        if (condition != null) {
            collection.addAll(visitor.visit(condition));
        }
        if (iteration != null) {
            collection.addAll(visitor.visit(iteration));
        }
        collection.addAll(statement.visitAllExpressions(visitor));
        return collection;
    }

    @Override
    public int pointValue() {
        int points = 0;
        points += declaration.pointValue() * 100;
        if (condition != null) {
            points += condition.pointValue() * 100;
        }
        if (iteration != null) {
            points += iteration.pointValue() * 100;
        }
        points += statement.pointValue();
        return points;
    }
}

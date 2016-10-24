package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class ExpressionStatement implements Statement {
    private Expression expression;

    @Override
    public String toCode() {
        return expression.toCode() + ";";
    }

    @Override
    public Set<String> getDependantVariables() {
        return expression.getDependentVariables();
    }

    @Override
    public boolean isCritical() {
        return !expression.getInvocations().isEmpty();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return expression.getGuaranteedChangedVariables();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return expression.getPotentiallyChangedVariables();
    }

    @Override
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return Collections.emptyList();
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        return visitor.visit(expression);
    }
}

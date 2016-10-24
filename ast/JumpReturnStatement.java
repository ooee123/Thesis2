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
public class JumpReturnStatement implements JumpStatementStrict {
    private Expression returnExpression;

    @Override
    public String toCode() {
        return "return " + returnExpression.toCode() + ";";
    }

    @Override
    public Set<String> getDependantVariables() {
        return returnExpression.getDependentVariables();
    }
/*
    @Override
    public Set<String> getChangedVariables() {
        return returnExpression.getGuaranteedChangedVariables();
    }
*/
    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return returnExpression.getPotentiallyChangedVariables();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return returnExpression.getGuaranteedChangedVariables();
    }

    @Override
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return Collections.emptyList();
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        return visitor.visit(returnExpression);
    }
}

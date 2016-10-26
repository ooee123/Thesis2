package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.List;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionParentheses implements PrimaryExpression {
    private Expression expression;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return expression.getGuaranteedChangedVariables();
    }

    public String toCode() {
        return "(" + expression.toCode() + " )";
    }

    @Override
    public Set<String> getDependentVariables() {
        return expression.getDependentVariables();
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        return expression.getInvocations();
    }

    @Override
    public Set<String> getVariables() {
        return expression.getVariables();
    }

    @Override
    public void visitNestedExpressions(Visitor<Void, Expression> visitor) {
        visitor.visit(expression);
    }

    @Override
    public int pointValue() {
        return expression.pointValue();
    }
}

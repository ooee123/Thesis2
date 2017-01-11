package ast.expression.impl;

import ast.expression.CastExpression;
import ast.expression.Expression;
import ast.expression.UnaryExpression;
import ast.expression.impl.PostfixExpressionInvocationImpl;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionUnaryOperatorImpl implements UnaryExpression {
    public enum UnaryOperator {
        ADDRESS_OF("&"),
        DEREFERENCE("*"),
        POSITIVE("+"),
        NEGATIVE("-"),
        BITWISE_NOT("~"),
        NOT("!");

        public final String token;

        UnaryOperator(String token) {
            this.token = token;
        }

        public static UnaryOperator toUnaryOperator(String token) {
            for (UnaryOperator unaryOperator : UnaryOperator.values()) {
                if (unaryOperator.token.equals(token)) {
                    return unaryOperator;
                }
            }
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    private UnaryOperator unaryOperator;
    private CastExpression castExpression;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return castExpression.getGuaranteedChangedVariables();
    }

    @Override
    public Set<String> getDependentVariables() {
        return castExpression.getDependentVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return castExpression.getInvocations();
    }

    public String toCode() {
        return unaryOperator.token + castExpression.toCode();
    }

    @Override
    public Set<String> getVariables() {
        return castExpression.getVariables();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(castExpression);
    }
}

package ast.expression.impl;

import ast.expression.CastExpression;
import ast.expression.Expression;
import ast.expression.MultiplicativeExpression;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class MultiplicativeExpressionImpl implements MultiplicativeExpression {
    public enum MultiplicativeOperator {
        MULTIPLY("*"),
        DIVIDE("/"),
        MODULO("%");

        public final String token;

        MultiplicativeOperator(String token) {
            this.token = token;
        }

        public static MultiplicativeOperator toMultiplicativeOperator(String token) {
            for (MultiplicativeOperator multiplicativeOperator : MultiplicativeOperator.values()) {
                if (multiplicativeOperator.token.equals(token)) {
                    return multiplicativeOperator;
                }
            }
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    private MultiplicativeExpression multiplicativeExpression;
    private MultiplicativeOperator multiplicativeOperator;
    private CastExpression castExpression;

    @Override
    public String toCode() {
        return multiplicativeExpression.toCode() + " " + multiplicativeOperator.token + " " + castExpression.toCode();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(multiplicativeExpression, castExpression);
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(multiplicativeExpression, castExpression);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(multiplicativeExpression, castExpression);
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = multiplicativeExpression.getVariables();
        variables.addAll(castExpression.getVariables());
        return variables;
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(multiplicativeExpression);
        visitor.visit(castExpression);
    }
}

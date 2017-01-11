package ast.expression.impl;

import ast.expression.Expression;
import ast.expression.PostfixExpression;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionIncrementImpl implements PostfixExpression {

    public enum PostfixOperator {
        POSTFIX_INCREMENT("++"),
        POSTFIX_DECREMENT("--");

        private final String token;

        PostfixOperator(String token) {
            this.token = token;
        }
    }

    private PostfixExpression postfixExpression;
    private PostfixOperator postfixOperator;

    @Override
    public String toCode() {
        return postfixExpression.toCode() + postfixOperator.token;
    }

    @Override
    public Set<String> getDependentVariables() {
        return postfixExpression.getVariables();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return postfixExpression.getVariables();
    }

    @Override
    public Set<String> getVariables() {
        return postfixExpression.getVariables();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return postfixExpression.getInvocations();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(postfixExpression);
    }
}

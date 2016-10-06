package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionImpl implements PostfixExpression {
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
}

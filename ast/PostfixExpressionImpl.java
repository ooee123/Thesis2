package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionImpl implements PostfixExpression {
    public enum PostfixOperator {
        POSTFIX_INCREMENT,
        POSTFIX_DECREMENT;
    }

    private PostfixExpression postfixExpression;
    private PostfixOperator postfixOperator;
}

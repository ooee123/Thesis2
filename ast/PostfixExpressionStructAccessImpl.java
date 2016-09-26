package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionStructAccessImpl implements PostfixExpression {
    public enum AccessOperator {
        DOT,
        ARROW;
    }

    private PostfixExpression postfixExpression;
    private String identifier;
}

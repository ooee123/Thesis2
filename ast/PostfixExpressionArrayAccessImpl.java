package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionArrayAccessImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private Expression arrayIndex;
}

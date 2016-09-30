package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionArrayAccessImpl implements PostfixExpression {
    private PostfixExpression postfixExpression;
    private Expression arrayIndex;

    @Override
    public Set<String> getLValues() {
        return postfixExpression.getLValues();
    }
}

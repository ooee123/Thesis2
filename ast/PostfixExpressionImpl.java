package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionImpl implements PostfixExpression {
    @Override
    public Set<String> getLValues() {
        return postfixExpression.getLValues();
    }

    public enum PostfixOperator {
        POSTFIX_INCREMENT,
        POSTFIX_DECREMENT;
    }

    private PostfixExpression postfixExpression;
    private PostfixOperator postfixOperator;
}

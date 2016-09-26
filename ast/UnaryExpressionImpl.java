package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionImpl implements UnaryExpression {
    public enum IncrementOperator {
        PREFIX_INCREMENT,
        PREFIX_DECREMENT;
    }

    private IncrementOperator incrementOperator;
    private UnaryExpression unaryExpression;
}

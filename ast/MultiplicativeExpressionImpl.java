package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class MultiplicativeExpressionImpl implements MultiplicativeExpression {
    public enum MultiplicativeOperator {
        MULTIPLY,
        DIVIDE,
        MODULO;
    }

    private MultiplicativeExpression multiplicativeExpression;
    private MultiplicativeOperator multiplicativeOperator;
    private CastExpression castExpression;
}

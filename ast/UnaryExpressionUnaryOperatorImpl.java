package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class UnaryExpressionUnaryOperatorImpl implements UnaryExpression {
    public enum UnaryOperator {
        ADDRESS_OF,
        DEREFERENCE,
        POSITIVE,
        NEGATIVE,
        BITWISE_NOT,
        NOT
    }

    private UnaryOperator unaryOperator;
    private CastExpression castExpression;
}

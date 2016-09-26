package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class EqualityExpressionImpl implements EqualityExpression {
    public enum EqualityOperator {
        EQUALS,
        NOT_EQUALS;
    }

    private EqualityExpressionImpl equalityExpression;
    private EqualityOperator equalityOperator;
    private RelationalExpression relationalExpression;
}

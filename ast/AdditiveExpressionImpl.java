package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImpl implements AdditiveExpression {
    public enum AdditiveOperator {
        PLUS,
        MINUS;
    }

    private AdditiveExpression additiveExpression;
    private MultiplicativeExpression multiplicativeExpression;
}

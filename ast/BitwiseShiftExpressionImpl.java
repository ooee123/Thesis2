package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class BitwiseShiftExpressionImpl implements BitwiseShiftExpression {
    public enum ShiftOperator {
        LEFT_SHIFT,
        RIGHT_SHIFT;
    }

    private BitwiseShiftExpression bitwiseShiftExpression;
    private AdditiveExpression additiveExpression;
}

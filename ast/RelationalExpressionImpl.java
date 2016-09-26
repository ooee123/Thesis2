package ast;

/**
 * Created by ooee on 9/26/16.
 */
public class RelationalExpressionImpl implements RelationalExpression {
    public enum RelationalOperator {
        LESS_THAN,
        GREATER_THAN,
        LESS_THAN_OR_EQUAL,
        GREATER_THAN_OR_EQUAL;
    }

    private RelationalExpression relationalExpression;
    private RelationalOperator relationalOperator;
    private BitwiseShiftExpression bitwiseShiftExpression;
}

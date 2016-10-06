package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class RelationalExpressionImpl implements RelationalExpression {
    public enum RelationalOperator {
        LESS_THAN("<"),
        GREATER_THAN(">"),
        LESS_THAN_OR_EQUAL("<="),
        GREATER_THAN_OR_EQUAL(">=");

        public final String token;

        RelationalOperator(String token) {
            this.token = token;
        }

        public static RelationalOperator toRelationalOperator(String token) {
            for (RelationalOperator relationalOperator : RelationalOperator.values()) {
                if (relationalOperator.token.equals(token)) {
                    return relationalOperator;
                }
            }
            return null;
        }
    }

    private RelationalExpression relationalExpression;
    private RelationalOperator relationalOperator;
    private BitwiseShiftExpression bitwiseShiftExpression;

    @Override
    public String toCode() {
        return relationalExpression.toCode() + relationalOperator.token + bitwiseShiftExpression.toCode();
    }
}

package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class MultiplicativeExpressionImpl implements MultiplicativeExpression {
    public enum MultiplicativeOperator {
        MULTIPLY("*"),
        DIVIDE("/"),
        MODULO("%");

        public final String token;

        MultiplicativeOperator(String token) {
            this.token = token;
        }

        public static MultiplicativeOperator toMultiplicativeOperator(String token) {
            for (MultiplicativeOperator multiplicativeOperator : MultiplicativeOperator.values()) {
                if (multiplicativeOperator.token.equals(token)) {
                    return multiplicativeOperator;
                }
            }
            return null;
        }
    }

    private MultiplicativeExpression multiplicativeExpression;
    private MultiplicativeOperator multiplicativeOperator;
    private CastExpression castExpression;
}

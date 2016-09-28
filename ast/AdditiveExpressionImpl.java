package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImpl implements AdditiveExpression {
    public enum AdditiveOperator {
        PLUS("+"),
        MINUS("-");

        public final String token;

        AdditiveOperator(String token) {
            this.token = token;
        }

        public static AdditiveOperator toAdditiveOperator(String token) {
            for (AdditiveOperator additiveOperator : AdditiveOperator.values()) {
                if (additiveOperator.token.equals(token)) {
                    return additiveOperator;
                }
            }
            return null;
        }
    }

    private AdditiveExpression additiveExpression;
    private AdditiveOperator additiveOperator;
    private MultiplicativeExpression multiplicativeExpression;
}

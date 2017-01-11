package ast.expression;

/**
 * Created by ooee on 9/26/16.
 */
public interface AdditiveExpression extends BitwiseShiftExpression {
    enum AdditiveOperator {
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
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }
}

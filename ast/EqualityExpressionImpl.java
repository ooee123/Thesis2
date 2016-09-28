package ast;

import lombok.Value;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class EqualityExpressionImpl implements EqualityExpression {
    public enum EqualityOperator {
        EQUALS("=="),
        NOT_EQUALS("!=");

        public final String token;

        EqualityOperator(String token) {
            this.token = token;
        }

        public static EqualityOperator toEqualityOperator(String token) {
            EqualityOperator[] equalityOperators = EqualityOperator.values();
            for (EqualityOperator equalityOperator : equalityOperators) {
                if (equalityOperator.token.equals(token)) {
                    return equalityOperator;
                }
            }
            return null;
        }
    }

    private EqualityExpression equalityExpression;
    private EqualityOperator equalityOperator;
    private RelationalExpression relationalExpression;
}

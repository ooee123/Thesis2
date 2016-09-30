package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class PostfixExpressionStructAccessImpl implements PostfixExpression {

    public enum AccessOperator {
        DOT("."),
        ARROW("->");

        public final String token;

        AccessOperator(String token) {
            this.token = token;
        }

        public static AccessOperator toAccessOperator(String token) {
            for (AccessOperator accessOperator: AccessOperator.values()) {
                if (accessOperator.token.equals(token)) {
                    return accessOperator;
                }
            }
            return null;
        }
    }

    private PostfixExpression postfixExpression;
    private AccessOperator accessOperator;
    private String identifier;

    @Override
    public Set<String> getLValues() {
        return postfixExpression.getLValues();
    }
}

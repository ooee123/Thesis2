package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.List;
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
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    private PostfixExpression postfixExpression;
    private AccessOperator accessOperator;
    private String identifier;

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return postfixExpression.getGuaranteedChangedVariables();
    }

    @Override
    public Set<String> getDependentVariables() {
        return postfixExpression.getDependentVariables();
    }

    public String toCode() {
        return postfixExpression.toCode() + accessOperator.token + identifier;
    }

    @Override
    public Set<String> getVariables() {
        return postfixExpression.getVariables();
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        return postfixExpression.getInvocations();
    }

    @Override
    public void visitNestedExpressions(Visitor<Void, Expression> visitor) {
        visitor.visit(postfixExpression);
    }
}

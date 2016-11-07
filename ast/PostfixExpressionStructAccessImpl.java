package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.HashSet;
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
        Set<String> guaranteedChangedVariables = new HashSet<>();
        Set<String> postfixGuaranteedChangedVariables = postfixExpression.getGuaranteedChangedVariables();
        for (String postfixGuaranteedChangedVariable : postfixGuaranteedChangedVariables) {
            guaranteedChangedVariables.add(postfixGuaranteedChangedVariable + accessOperator.token + identifier);
        }
        throw new IllegalArgumentException("In");
        //return guaranteedChangedVariables;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        Set<String> postfixDependentVariables = postfixExpression.getDependentVariables();
        for (String postfixDependentVariable : postfixDependentVariables) {
            dependentVariables.add(postfixDependentVariable + accessOperator.token + identifier);
        }
        return dependentVariables;
    }

    public String toCode() {
        return postfixExpression.toCode() + accessOperator.token + identifier;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        Set<String> postfixVariables = postfixExpression.getVariables();
        for (String postfixVariable : postfixVariables) {
            variables.add(postfixVariable + accessOperator.token + identifier);
        }
        return variables;
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return postfixExpression.getInvocations();
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(postfixExpression);
    }
}

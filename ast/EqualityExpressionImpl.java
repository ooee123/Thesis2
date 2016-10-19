package ast;

import lombok.Value;

import java.util.Set;

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

    @Override
    public String toCode() {
        return equalityExpression.toCode() + equalityOperator.token + relationalExpression.toCode();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGet(exp -> exp.getGuaranteedChangedVariables(), equalityExpression, relationalExpression);
    }

    @Override
    public Set<String> getVariables() {
        return multiGet(exp -> exp.getVariables(), equalityExpression, relationalExpression);
        /*
        Set<String> variables = new HashSet<>();
        variables.addAll(equalityExpression.getVariables());
        variables.addAll(relationalExpression.getVariables());
        return variables;
        */
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGet(exp -> exp.getDependentVariables(), equalityExpression, relationalExpression);
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGet(exp -> exp.getInvocations(), equalityExpression, relationalExpression);
    }
}

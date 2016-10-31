package ast;

import lombok.NonNull;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

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
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    @NonNull private RelationalExpression relationalExpression;
    @NonNull private RelationalOperator relationalOperator;
    @NonNull private BitwiseShiftExpression bitwiseShiftExpression;

    @Override
    public String toCode() {
        return relationalExpression.toCode() + relationalOperator.token + bitwiseShiftExpression.toCode();
    }

    @Override
    public Set<PostfixExpressionInvocationImpl> getInvocations() {
        return multiGetInvocations(relationalExpression, bitwiseShiftExpression);
    }

    @Override
    public Set<String> getVariables() {
        return multiGetVariables(relationalExpression, bitwiseShiftExpression);
    }

    @Override
    public Set<String> getDependentVariables() {
        return multiGetDependentVariables(relationalExpression, bitwiseShiftExpression);
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return multiGetGuaranteedChangedVariables(relationalExpression, bitwiseShiftExpression);
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(relationalExpression);
        visitor.visit(bitwiseShiftExpression);
    }
}

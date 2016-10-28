package ast;

import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class BitwiseShiftExpressionImpl implements BitwiseShiftExpression {
    public enum ShiftOperator {
        LEFT_SHIFT("<<"),
        RIGHT_SHIFT(">>");

        public final String token;

        ShiftOperator(String token) {
            this.token = token;
        }

        public static ShiftOperator toShiftOperator(String token) {
            for (ShiftOperator shiftOperator : ShiftOperator.values()) {
                if (shiftOperator.token.equals(token)) {
                    return shiftOperator;
                }
            }
            throw new IllegalArgumentException("Token " + token + " not recognized");
        }
    }

    private BitwiseShiftExpression bitwiseShiftExpression;
    private ShiftOperator shiftOperator;
    private AdditiveExpression additiveExpression;

    @Override
    public String toCode() {
        return bitwiseShiftExpression.toCode() + shiftOperator.token + additiveExpression.toCode();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(bitwiseShiftExpression.getVariables());
        variables.addAll(additiveExpression.getVariables());
        return variables;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(bitwiseShiftExpression.getDependentVariables());
        dependentVariables.addAll(additiveExpression.getDependentVariables());
        return dependentVariables;
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        List<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();
        invocations.addAll(bitwiseShiftExpression.getInvocations());
        invocations.addAll(additiveExpression.getInvocations());
        return invocations;
    }

    @Override
    public void visitNestedExpressions(Visitor<Expression> visitor) {
        visitor.visit(bitwiseShiftExpression);
        visitor.visit(additiveExpression);
    }
}

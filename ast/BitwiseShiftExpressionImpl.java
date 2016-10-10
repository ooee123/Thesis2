package ast;

import lombok.Value;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

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
            return null;
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
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(bitwiseShiftExpression.getVariables());
        variables.addAll(additiveExpression.getVariables());
        return variables;
    }
}

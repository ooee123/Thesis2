package ast;

import lombok.Value;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
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
            return null;
        }
    }

    private RelationalExpression relationalExpression;
    private RelationalOperator relationalOperator;
    private BitwiseShiftExpression bitwiseShiftExpression;

    @Override
    public String toCode() {
        return relationalExpression.toCode() + relationalOperator.token + bitwiseShiftExpression.toCode();
    }

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        variables.addAll(relationalExpression.getVariables());
        variables.addAll(bitwiseShiftExpression.getVariables());
        return variables;
    }
}

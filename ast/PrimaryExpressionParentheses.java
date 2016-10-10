package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionParentheses implements PrimaryExpression, Assigning {
    private Expression expression;

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }

    public String toCode() {
        return "(" + expression.toCode() + " )";
    }

    @Override
    public Set<String> getRightVariables() {
        if (expression instanceof Assigning) {
            return ((Assigning) expression).getRightVariables();
        } else {
            return Collections.emptySet();
        }
    }

    @Override
    public Set<String> getVariables() {
        return expression.getVariables();
    }
}

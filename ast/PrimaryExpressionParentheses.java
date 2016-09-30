package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by kly04 on 9/26/16.
 */
@Value
public class PrimaryExpressionParentheses implements PrimaryExpression {
    private Expression expression;

    @Override
    public Set<String> getLValues() {
        return Collections.emptySet();
    }
}

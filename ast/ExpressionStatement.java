package ast;

import lombok.Value;

import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class ExpressionStatement implements Statement {
    private Expression expression;

    @Override
    public String toCode() {
        return expression.toCode() + ";";
    }
}

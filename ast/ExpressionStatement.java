package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class ExpressionStatement implements Statement {
    private Expression expression;
}

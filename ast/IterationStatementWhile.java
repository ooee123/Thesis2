package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class IterationStatementWhile implements IterationStatement {
    private Expression condition;
    private Statement statement;
}

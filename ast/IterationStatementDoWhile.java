package ast;

import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class IterationStatementDoWhile implements IterationStatement {
    private Statement statement;
    private Expression condition;
}

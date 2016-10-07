package ast;

import lombok.AllArgsConstructor;
import lombok.Data;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementDoWhile implements IterationStatement {
    private Statement statement;
    private Expression condition;

    @Override
    public String toCode() {
        return String.format("do %s while (%s)", statement.toCode(), condition.toCode());
    }
}

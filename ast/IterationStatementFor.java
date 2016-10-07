package ast;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;

/**
 * Created by ooee on 9/25/16.
 */
@Data
@AllArgsConstructor
public class IterationStatementFor implements IterationStatement {
    private Expression initial;
    private Expression condition;
    private Expression iteration;
    @NonNull private Statement statement;

    @Override
    public String toCode() {
        String initialString = "", conditionString = "", iterationString = "";
        if (initial != null) {
            initialString = initial.toCode();
        }
        if (condition != null) {
            conditionString = condition.toCode();
        }
        if (iteration != null) {
            iterationString = iteration.toCode();
        }
        return String.format("for (%s; %s; %s) %s", initialString, conditionString, iterationString, statement.toCode());
    }
}

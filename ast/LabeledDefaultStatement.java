package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 10/6/16.
 */
@Value
public class LabeledDefaultStatement implements LabeledStatement {
    private Statement statement;

    @Override
    public String toCode() {
        return "default: " + statement.toCode();
    }

    @Override
    public Set<String> getDependantVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getChangedVariables() {
        return Collections.emptySet();
    }
}

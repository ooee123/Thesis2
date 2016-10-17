package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpContinueStatement implements JumpStatementStrict {
    @Override
    public String toCode() {
        return "continue;";
    }

    @Override
    public Set<String> getDependantVariables() {
        return Collections.emptySet();
    }

    @Override
    public Set<String> getChangedVariables() {
        return Collections.emptySet();
    }

    @Override
    public boolean isCritical() {
        return true;
    }
}

package ast;

import lombok.Value;

import java.util.Collections;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpBreakStatement implements JumpStatement {
    @Override
    public String toCode() {
        return "break;";
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

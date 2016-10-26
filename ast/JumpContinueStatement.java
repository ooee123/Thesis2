package ast;

import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Collection;
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
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        return Sets.newHashSet();
    }

    @Override
    public boolean isCritical() {
        return true;
    }

    @Override
    public <T> Collection<T> visitEachStatement(Visitor<T, Statement> visitor) {
        return Collections.emptyList();
    }

    @Override
    public <T> Collection<T> visitAllExpressions(Visitor<T, Expression> visitor) {
        return Collections.emptyList();
    }

    @Override
    public int pointValue() {
        return 0;
    }
}

package ast.statement.impl;

import ast.expression.Expression;
import ast.statement.JumpStatementStrict;
import ast.statement.Statement;
import com.google.common.collect.Sets;
import lombok.Value;
import visitor.Visitor;

import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class JumpContinueStatement implements JumpStatementStrict {
    @Override
    public String toCode(boolean showOriginalLine) {
        return toCode();
    }

    public String toCode() {
        return "continue;";
    }

    @Override
    public boolean hasJump() {
        return true;
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
    public void visitEachStatement(Visitor<Statement> visitor) {
        return;
    }

    @Override
    public void visitOwnedExpressions(Visitor<Expression> visitor) {
        return;
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        return;
    }
}

package ast;

import IdentifierNormalizer.FunctionIdentifierNormalizerVisitor;
import com.google.common.collect.Lists;
import lombok.Data;
import lombok.NonNull;
import visitor.Visitor;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/25/16.
 */
@Data
public class SelectionStatementIf implements SelectionStatement, CanContainStatements {
    @NonNull private Expression condition;
    @NonNull private Statement thenStatement;
    private Statement elseStatement;

    public SelectionStatementIf(Expression condition, Statement thenStatement, Statement elseStatement) {
        this.condition = condition;
        this.thenStatement = thenStatement;
        this.elseStatement = elseStatement;
    }

    public SelectionStatementIf(Expression condition, Statement thenStatement) {
        this(condition, thenStatement, null);
    }

    @Override
    public String toCode() {
        String base = String.format("if (%s) %s", condition.toCode(), thenStatement.toCode());
        if (elseStatement != null) {
            return String.format("%s else %s", base, elseStatement.toCode());
        } else {
            return base;
        }
    }

    @Override
    public Set<String> getDependantVariables() {
        Set<String> dependentVariables = new HashSet<>();
        dependentVariables.addAll(condition.getDependentVariables());
        dependentVariables.addAll(thenStatement.getDependantVariables());
        if (elseStatement != null) {
            dependentVariables.addAll(elseStatement.getDependantVariables());
        }
        return dependentVariables;
    }

    @Override
    public Collection<Statement> getStatementNodes() {
        Collection<Statement> statements = Lists.newArrayList(thenStatement);
        if (elseStatement != null) {
            statements.add(elseStatement);
        }
        return statements;
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        throw new IllegalArgumentException("Ever run?");
        /*
        if (elseStatement != null) {
            Set<String> changedVariables = new HashSet<>(thenStatement.getGuaranteedChangedVariables());
            changedVariables.retainAll(elseStatement.getGuaranteedChangedVariables());
            changedVariables.addAll(condition.getGuaranteedChangedVariables());
            return changedVariables;
        } else {
            return condition.getGuaranteedChangedVariables();
        }
        */
    }

    @Override
    public Set<String> getPotentiallyChangedVariables() {
        throw new IllegalArgumentException("Ever run?");
        /*
        Set<String> potentiallyChangedVariables = new HashSet<>();
        if (elseStatement != null) {
            Set<String> thenGuaranteed = new HashSet<>(thenStatement.getGuaranteedChangedVariables());
            Set<String> elseGuaranteed = new HashSet<>(elseStatement.getGuaranteedChangedVariables());
            Set<String> intersection = new HashSet<>(thenGuaranteed);
            intersection.retainAll(elseGuaranteed);
            potentiallyChangedVariables.addAll(thenGuaranteed);
            potentiallyChangedVariables.addAll(elseGuaranteed);
            potentiallyChangedVariables.removeAll(intersection);

            potentiallyChangedVariables.addAll(thenStatement.getPotentiallyChangedVariables());
            potentiallyChangedVariables.addAll(elseStatement.getPotentiallyChangedVariables());
            potentiallyChangedVariables.addAll(condition.getGuaranteedChangedVariables());
        } else {
            potentiallyChangedVariables.addAll(condition.getPotentiallyChangedVariables());
            potentiallyChangedVariables.addAll(thenStatement.getGuaranteedChangedVariables());
            potentiallyChangedVariables.addAll(thenStatement.getPotentiallyChangedVariables());
        }
        return potentiallyChangedVariables;
        */
    }

    @Override
    public boolean isCritical() {
        boolean elseCritical = false;
        if (elseStatement != null) {
            elseCritical = elseStatement.isCritical();
        }
        return condition.getInvocations().size() > 0 || thenStatement.isCritical() || elseCritical;
    }

    @Override
    public void visitEachStatement(Visitor<Statement> visitor) {
        visitor.visit(thenStatement);
        if (elseStatement != null) {
            visitor.visit(elseStatement);
        }
    }

    @Override
    public void visitAllExpressions(Visitor<Expression> visitor) {
        visitor.visit(condition);
        thenStatement.visitAllExpressions(visitor);
        if (elseStatement != null) {
            elseStatement.visitAllExpressions(visitor);
        }
    }
}

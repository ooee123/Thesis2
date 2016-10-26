package ast;

import com.google.common.collect.Sets;
import visitor.Visitor;

import java.util.*;

/**
 * Created by ooee on 9/25/16.
 */
public interface Expression extends BaseElement {
    Set<String> getVariables();

    Set<String> getDependentVariables();

    Set<String> getGuaranteedChangedVariables();

    default Set<String> getPotentiallyChangedVariables() {
        return Sets.newHashSet();
    }

    List<PostfixExpressionInvocationImpl> getInvocations();

    void visitNestedExpressions(Visitor<Void, Expression> visitor);

    //int pointValue();

    default int pointValue() {
        return 0;
    }

    default <T> Set<T> multiGetSet(java.util.function.Function<Expression, Set<T>> f, Expression... expressions) {
        return multiGetCollection(new HashSet<>(), f, expressions);
    }

    default <T> List<T> multiGetList(java.util.function.Function<Expression, List<T>> f, Expression... expressions) {
        return multiGetCollection(new ArrayList<>(), f, expressions);
    }

    default <T, C extends Collection<T>> C multiGetCollection(C collection, java.util.function.Function<Expression, C> f, Expression... expressions) {
        for (Expression expression : expressions) {
            collection.addAll(f.apply(expression));
        }
        return collection;
    }

    default Set<String> multiGetVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getVariables(), expressions);
    }

    default Set<String> multiGetDependentVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getDependentVariables(), expressions);
    }

    default Set<String> multiGetGuaranteedChangedVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getGuaranteedChangedVariables(), expressions);
    }

    default Set<String> multiGetPotentiallyChangedVariables(Expression... expressions) {
        return multiGetSet(exp -> exp.getPotentiallyChangedVariables(), expressions);
    }

    default List<PostfixExpressionInvocationImpl> multiGetInvocations(Expression... expressions) {
        return multiGetList(exp -> exp.getInvocations(), expressions);
    }
}

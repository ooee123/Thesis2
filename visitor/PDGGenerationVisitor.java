package visitor;

import ast.*;
import ast.Function;
import ast.type.PointerType;
import com.google.common.collect.Sets;
import lombok.AllArgsConstructor;
import lombok.Value;
import pdg.*;

import java.util.*;
import java.util.stream.Collectors;

@Value
public class PDGGenerationVisitor {

    @Value
    private class Returns<T extends BlockItem> {
        Dependencies dependencies;
        PDGNode<T> pdgNode;
    }

    public void link(PDGNode fromNode, PDGNode toNode) {
        fromNode.getIsADependencyFor().add(toNode);
        toNode.getDependsOn().add(fromNode);
    }

    public void printAdjacencyMatrix(Map<String, PDGNode> allNodes) {
        int i = 0;
        String label;
        System.out.println("Adjacency Matrix");
        Map<PDGNode, String> labels = new IdentityHashMap<>();
        for (PDGNode pdgNode : allNodes.values()) {
            label = String.valueOf(Character.valueOf((char) ('A' + i)));
            labels.put(pdgNode, label);
            System.out.println(label + " := " + pdgNode.getBlockItem().toCode());
            i++;
        }

        for (PDGNode pdgNode : allNodes.values()) {
            String source = labels.get(pdgNode);
            System.out.print(source + ": ");
            List<String> destLabels = (List<String>)pdgNode.getIsADependencyFor().stream().map(node -> labels.get(node)).collect(Collectors.toList());
            System.out.println(String.join(", ", destLabels));
        }
        System.out.println("End Matrix");
    }


    @Value
    @AllArgsConstructor
    private class Dependencies {
        private Set<String> dependentVariables;
        private Set<String> guaranteedChangedVariables;
        private Set<String> potentiallyChangedVariables;

        Dependencies() {
            dependentVariables = Collections.emptySet();
            guaranteedChangedVariables = Collections.emptySet();
            potentiallyChangedVariables = Collections.emptySet();
        }

        Dependencies add(Dependencies d) {
            Set<String> otherDependentVariables = new HashSet<>(d.dependentVariables);
            otherDependentVariables.removeAll(this.guaranteedChangedVariables);
            this.dependentVariables.addAll(otherDependentVariables);
            this.potentiallyChangedVariables.removeAll(d.guaranteedChangedVariables);
            this.guaranteedChangedVariables.removeAll(d.potentiallyChangedVariables);
            this.guaranteedChangedVariables.addAll(d.guaranteedChangedVariables);
            this.potentiallyChangedVariables.addAll(d.potentiallyChangedVariables);
            return this;
        }
    }

    private Dependencies nullableExpression(Expression expression, Dependencies dependencies) {
        if (expression != null) {
            Set<String> rightVariables = expression.getDependentVariables();
            rightVariables.removeAll(dependencies.guaranteedChangedVariables);
            dependencies.dependentVariables.addAll(rightVariables);
            dependencies.guaranteedChangedVariables.removeAll(expression.getPotentiallyChangedVariables());
            dependencies.potentiallyChangedVariables.removeAll(expression.getGuaranteedChangedVariables());
            dependencies.guaranteedChangedVariables.addAll(expression.getGuaranteedChangedVariables());
            dependencies.potentiallyChangedVariables.addAll(expression.getPotentiallyChangedVariables());
        }
        return dependencies;
    }

    public void visit(Program p) {
        for (Function function : p.getFunction()) {
            Collection<PDGNode<? extends BlockItem>> visit = visit(function);
        }
    }

    public Collection<PDGNode<? extends BlockItem>> visit(Function function) {
        Returns<CompoundStatement> returns = visit(function.getCompoundStatement());
        for (Parameter parameter : function.getParameterList().getParameters()) {
            // If the type is a pointer, mark it as changed.
            //functionDependencies.


            if (parameter.getType() instanceof PointerType && returns.dependencies.guaranteedChangedVariables.contains(parameter.getFormalParameterName())) {
                // Pointer value changed

            }
        }
        //functionDependencies.getGuaranteedChangedVariables()
        return ((PDGNodeCompoundStatement) returns.pdgNode).getBody();
    }

    public Returns<JumpReturnStatement> visit(JumpReturnStatement statement) {
        return new Returns<>(visit(statement.getReturnExpression()), new PDGNodeReturn(statement, true));
    }

    public Returns<JumpBreakStatement> visit(JumpBreakStatement statement) {
        return new Returns<>(new Dependencies(), new PDGNodeBreak(statement, true));
    }

    public Returns<JumpContinueStatement> visit(JumpContinueStatement statement) {
        return new Returns<>(new Dependencies(), new PDGNodeContinue(statement, true));
    }

    public Returns<? extends Statement> visit(Statement statement) {
        if (statement instanceof CompoundStatement) {
            return visit(((CompoundStatement) statement));
        } else if (statement instanceof ExpressionStatement) {
            return visit(((ExpressionStatement) statement));
        } else if (statement instanceof SelectionStatementIf) {
            return visit(((SelectionStatementIf) statement));
        } else if (statement instanceof IterationStatementFor) {
            return visit(((IterationStatementFor) statement));
        } else if (statement instanceof IterationStatementDeclareFor) {
            return visit(((IterationStatementDeclareFor) statement));
        } else if (statement instanceof IterationStatementWhile) {
            return visit(((IterationStatementWhile) statement));
        } else if (statement instanceof IterationStatementDoWhile) {
            return visit(((IterationStatementDoWhile) statement));
        } else if (statement instanceof JumpReturnStatement) {
            return visit(((JumpReturnStatement) statement));
        } else if (statement instanceof JumpBreakStatement) {
            return visit(((JumpBreakStatement) statement));
        } else if (statement instanceof JumpContinueStatement) {
            return visit(((JumpContinueStatement) statement));
        } else {
            return null;
        }
    }

    private Returns<? extends BlockItem> visit(BlockItem blockItem) {
         if (blockItem instanceof Declaration) {
            return visit(((Declaration) blockItem));
        } else {
            return visit((Statement) blockItem);
        }
    }

    private Returns<IterationStatementFor> visit(IterationStatementFor statement) {
        Dependencies dependencies = new Dependencies();
        nullableExpression(statement.getInitial(), dependencies);
        nullableExpression(statement.getCondition(), dependencies);
        Returns<? extends Statement> returns = visit(statement.getStatement());

        Dependencies bodyDependencies = returns.dependencies;
        bodyDependencies.dependentVariables.removeAll(dependencies.getPotentiallyChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());

        Dependencies iterationDependencies = nullableExpression(statement.getIteration(), new Dependencies());
        dependencies.dependentVariables.addAll(iterationDependencies.getDependentVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getPotentiallyChangedVariables());
        return new Returns<>(dependencies, new PDGNodeFor(statement, returns.getPdgNode()));
    }

    private Returns<IterationStatementDeclareFor> visit(IterationStatementDeclareFor statement) {
        Set<String> declaredVariables = new HashSet<>();
        Dependencies dependencies = new Dependencies();
        for (Declaration.DeclaredVariable declaredVariable : statement.getDeclaration().getDeclaredVariables()) {
            if (declaredVariable.getInitializer() != null) {
                nullableExpression(declaredVariable.getInitializer(), dependencies);
                declaredVariables.add(declaredVariable.getIdentifier());
            }
        }
        nullableExpression(statement.getCondition(), dependencies);
        Returns<? extends Statement> returns = visit(statement.getStatement());
        Dependencies bodyDependencies = returns.dependencies;

        bodyDependencies.dependentVariables.removeAll(dependencies.getPotentiallyChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());

        Dependencies iterationDependencies = nullableExpression(statement.getIteration(), new Dependencies());
        dependencies.dependentVariables.addAll(iterationDependencies.getDependentVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(iterationDependencies.getPotentiallyChangedVariables());

        dependencies.dependentVariables.removeAll(declaredVariables);
        dependencies.potentiallyChangedVariables.removeAll(declaredVariables);
        dependencies.guaranteedChangedVariables.removeAll(declaredVariables);
        return new Returns<>(dependencies, new PDGNodeDeclareFor(statement, returns.pdgNode));
    }

    private Returns<IterationStatementWhile> visit(IterationStatementWhile statement) {
        Dependencies dependencies = visit(statement.getCondition());
        Returns<? extends Statement> returns = visit(statement.getStatement());
        Dependencies bodyDependencies = returns.dependencies;
        bodyDependencies.dependentVariables.removeAll(dependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getGuaranteedChangedVariables());
        dependencies.potentiallyChangedVariables.addAll(bodyDependencies.getPotentiallyChangedVariables());
        dependencies.dependentVariables.addAll(bodyDependencies.getDependentVariables());
        return new Returns<>(dependencies, new PDGNodeWhile(statement, returns.getPdgNode()));
    }

    private Returns<IterationStatementDoWhile> visit(IterationStatementDoWhile statement) {
        Dependencies dependencies = visit(statement.getCondition());
        Returns<? extends Statement> returns = visit(statement.getStatement());

        dependencies.add(returns.dependencies);
        return new Returns<>(dependencies, new PDGNodeDoWhile(statement, returns.getPdgNode()));
    }

    private Returns<ExpressionStatement> visit(ExpressionStatement statement) {
        boolean hasInvocations = !statement.getExpression().getInvocations().isEmpty();
        return new Returns<>(visit(statement.getExpression()), new PDGNodeExpressionStatement(statement, hasInvocations));
    }

    private Dependencies visit(Expression expression) {
        Set<String> dependentVariables = expression.getDependentVariables();
        Set<String> changedVariables = expression.getGuaranteedChangedVariables();
        Set<String> potentialVariables = expression.getPotentiallyChangedVariables();
        return new Dependencies(dependentVariables, changedVariables, potentialVariables);
    }

    private Returns<Declaration> visit(Declaration declaration) {
        Set<String> declaredVariables = new HashSet<>();
        Set<String> dependentVariables = new HashSet<>();
        Set<String> guaranteedChangedVariables = new HashSet<>();
        Set<String> potentiallyChangedVariables = new HashSet<>();
        for (Declaration.DeclaredVariable declaredVariable : declaration.getDeclaredVariables()) {
            declaredVariables.add(declaredVariable.getIdentifier());
            if (declaredVariable.getInitializer() != null) {
                Dependencies dependencies = visit(declaredVariable.getInitializer());
                dependencies.dependentVariables.removeAll(declaredVariables);
                dependentVariables.addAll(dependencies.dependentVariables);
                guaranteedChangedVariables.addAll(dependencies.getGuaranteedChangedVariables());
                potentiallyChangedVariables.addAll(dependencies.getPotentiallyChangedVariables());
                guaranteedChangedVariables.add(declaredVariable.getIdentifier());
            }
        }
        return new Returns<>(new Dependencies(dependentVariables, guaranteedChangedVariables, potentiallyChangedVariables), new PDGNodeDeclaration(declaration));
    }

    private Returns<CompoundStatement> visit(CompoundStatement statement) {
        Map<BlockItem, PDGNode<? extends BlockItem>> allNodes = new IdentityHashMap<>();
        //Map<String, Collection<BlockItem>> lastAssigned = new HashMap<>();
        Map<String, Collection<PDGNode<? extends BlockItem>>> lastAssigned = new HashMap<>();
        Set<String> dependentVariables = new HashSet<>();
        Set<String> locallyDeclaredVariables = new HashSet<>();
        for (BlockItem blockItem : statement.getBlockItems()) {
            //System.out.println(blockItem.toCode());
            Returns returns = visit(blockItem);
            allNodes.put(blockItem, returns.getPdgNode());
            //System.out.println(returns.dependencies);
            for (String usedVariable : returns.dependencies.getDependentVariables()) {
                if (!lastAssigned.containsKey(usedVariable)) {
                    dependentVariables.add(usedVariable);
                } else {
                    Collection<BlockItem> froms = lastAssigned.get(usedVariable).stream().map(blk -> blk.getBlockItem()).collect(Collectors.toList());
                    PDGNode toNode = allNodes.get(blockItem);
                    for (BlockItem from : froms) {
                        PDGNode fromNode = allNodes.get(from);
                        link(fromNode, toNode);
                    }
                }
            }
            for (String changedVariables : returns.dependencies.getGuaranteedChangedVariables()) {
                lastAssigned.put(changedVariables, Sets.newHashSet(returns.getPdgNode()));
            }
            for (String potentiallyChangedVariable : returns.dependencies.getPotentiallyChangedVariables()) {
                if (!lastAssigned.containsKey(potentiallyChangedVariable)) {
                    lastAssigned.put(potentiallyChangedVariable, new ArrayList<>());
                }
                lastAssigned.get(potentiallyChangedVariable).add(returns.getPdgNode());
            }
            if (blockItem instanceof Declaration) {
                for (Declaration.DeclaredVariable declaredVariable : ((Declaration) blockItem).getDeclaredVariables()) {
                    lastAssigned.put(declaredVariable.getIdentifier(), Sets.newHashSet(returns.getPdgNode()));
                    locallyDeclaredVariables.add(declaredVariable.getIdentifier());
                }
            }
        }
        HashSet<String> changed = new HashSet<>(lastAssigned.keySet());
        changed.removeAll(locallyDeclaredVariables);
        Set<String> guaranteedChangedVariables = changed.stream().filter(str -> lastAssigned.get(str).size() == 1).collect(Collectors.toSet());
        Set<String> potentiallyChangedVariables = changed.stream().filter(str -> lastAssigned.get(str).size() > 1).collect(Collectors.toSet());
        if (guaranteedChangedVariables.size() + potentiallyChangedVariables.size() != changed.size()) {
            System.err.println("Huh, doesn't add up");
        }
        //printAdjacencyMatrix(allNodes);
        return new Returns<>(new Dependencies(dependentVariables, guaranteedChangedVariables, potentiallyChangedVariables), new PDGNodeCompoundStatement(statement, allNodes.values(), lastAssigned));
    }

    private Returns<SelectionStatementIf> visit(SelectionStatementIf statement) {
        Dependencies conditionDependencies = visit(statement.getCondition());
        Set<String> dependentVariables = new HashSet<>(conditionDependencies.getDependentVariables());
        Set<String> guaranteedChangedVariables = new HashSet<>(conditionDependencies.getGuaranteedChangedVariables());
        Set<String> potentiallyChangedVariables = new HashSet<>(conditionDependencies.getPotentiallyChangedVariables());

        Statement thenStatement = statement.getThenStatement();
        Returns<? extends Statement> thenReturns = visit(thenStatement);
        Dependencies thenDependencies = thenReturns.dependencies;
        thenDependencies.dependentVariables.removeAll(guaranteedChangedVariables);
        dependentVariables.addAll(thenDependencies.dependentVariables);
        potentiallyChangedVariables.addAll(thenDependencies.getPotentiallyChangedVariables());
        PDGNode<? extends Statement> elseNode = null;
        if (statement.getElseStatement() != null) {
            Set<String> branchesChangedVariables = new HashSet<>();
            Returns<? extends Statement> elseReturns = visit(statement.getElseStatement());
            elseNode = elseReturns.pdgNode;
            Dependencies elseDependencies = elseReturns.dependencies;
            potentiallyChangedVariables.addAll(elseDependencies.getPotentiallyChangedVariables());
            elseDependencies.dependentVariables.removeAll(guaranteedChangedVariables);
            dependentVariables.addAll(elseDependencies.dependentVariables);
            branchesChangedVariables.addAll(elseDependencies.getGuaranteedChangedVariables());
            branchesChangedVariables.retainAll(thenDependencies.getGuaranteedChangedVariables());
            guaranteedChangedVariables.addAll(branchesChangedVariables);
        } else {
            potentiallyChangedVariables.addAll(thenDependencies.getGuaranteedChangedVariables());
        }
        return new Returns<>(new Dependencies(dependentVariables, guaranteedChangedVariables, potentiallyChangedVariables), new PDGNodeIf(statement, thenReturns.pdgNode, elseNode));

    }

    private void printLastAssigned(Map<String, BlockItem> map) {
        for (Map.Entry<String, BlockItem> stringBlockItemEntry : map.entrySet()) {
            System.out.println(stringBlockItemEntry.getKey() + ": " + stringBlockItemEntry.getValue().toCode());
        }
    }
}
